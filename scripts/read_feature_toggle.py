import yaml
import os

def read_feature_toggle_build_cmd():
    cwd = os.getcwd();
    os.chdir("../")
    file_yaml = "src/config/feature_toggle.yaml"
    cmd = ""
    with open(file=file_yaml, mode="r", encoding="utf-8") as file:
        crf = file.read()

        yaml_data = yaml.load(stream=crf, Loader=yaml.FullLoader)
        keys = list(yaml_data)
        for key in keys: 
            print(yaml_data[key])
            if yaml_data[key]:
                cmd += " -D" + key +"=true"

        file.close()
    os.chdir(cwd)
    return cmd