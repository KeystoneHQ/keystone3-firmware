import os
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import build


class BuildImageCacheTests(unittest.TestCase):
    def setUp(self):
        temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(temporary_directory.cleanup)
        self.root = Path(temporary_directory.name)
        original_directory = Path.cwd()
        os.chdir(self.root)
        self.addCleanup(os.chdir, original_directory)
        self.cache_file = self.root / 'src/ui/gui_assets/images_hash.txt'
        self.cache_file.parent.mkdir(parents=True)
        self.write_image('a.png', b'first image')
        self.write_image('z.PNG', b'second image')
        self.write_image('a/nested.png', b'first nested image')
        self.write_image('z/nested.png', b'second nested image')

        self.converter = mock.Mock()
        converter_module = types.SimpleNamespace(ImgConverter=self.converter)
        patcher = mock.patch.dict(sys.modules, {'img_converter': converter_module})
        patcher.start()
        self.addCleanup(patcher.stop)
        self.original_walk = os.walk

    def write_image(self, name, content):
        path = self.root / 'images' / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)

    def walk_in_order(self, reverse_files=False, reverse_directories=False):
        def walk(directory):
            for root, directories, files in self.original_walk(directory):
                directories.sort(reverse=reverse_directories)
                yield root, directories, sorted(files, reverse=reverse_files)
        return walk

    def populate_cache(self):
        with mock.patch.object(build.os, 'walk', self.walk_in_order()):
            build.build_img_to_c_file()
        self.converter.assert_called_once()
        self.converter.reset_mock()
        return self.cache_file.read_text()

    def test_file_order_does_not_invalidate_cache(self):
        previous_hash = self.populate_cache()
        with mock.patch.object(build.os, 'walk', self.walk_in_order(reverse_files=True)):
            build.build_img_to_c_file()
        self.assertEqual(previous_hash, self.cache_file.read_text())
        self.converter.assert_not_called()

    def test_directory_order_does_not_invalidate_cache(self):
        previous_hash = self.populate_cache()
        with mock.patch.object(build.os, 'walk', self.walk_in_order(reverse_directories=True)):
            build.build_img_to_c_file()
        self.assertEqual(previous_hash, self.cache_file.read_text())
        self.converter.assert_not_called()

    def test_changed_png_still_invalidates_cache(self):
        previous_hash = self.populate_cache()
        self.write_image('z.PNG', b'changed image')
        build.build_img_to_c_file()
        self.assertNotEqual(previous_hash, self.cache_file.read_text())
        self.converter.assert_called_once()

    def test_non_png_file_does_not_invalidate_cache(self):
        previous_hash = self.populate_cache()
        self.write_image('notes.txt', b'not an image')
        build.build_img_to_c_file()
        self.assertEqual(previous_hash, self.cache_file.read_text())
        self.converter.assert_not_called()


if __name__ == '__main__':
    unittest.main()
