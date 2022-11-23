import hashlib
import random

import pandas as pd
import requests


class BaiDuTranslate:
    def __init__(self, appKey, appSecret):
        self.url = 'https://fanyi-api.baidu.com/api/trans/vip/translate'
        self.appid = appKey
        self.secretKey = appSecret
        self.fromLang = 'auto'
        self.toLang = 'cht'
        self.salt = random.randint(32768, 65536)
        self.header = {'Content-Type': 'application/x-www-form-urlencoded'}

    def Translate(self, text):
        sign = self.appid + text + str(self.salt) + self.secretKey
        md = hashlib.md5()
        md.update(sign.encode(encoding='utf-8'))
        sign = md.hexdigest()
        data = {
            "appid": self.appid,
            "q": text,
            "from": self.fromLang,
            "to": self.toLang,
            "salt": self.salt,
            "sign": sign
        }
        response = requests.post(self.url, params = data, headers = self.header)
        text = response.json()
        results = text['trans_result'][0]['dst']
        return results


if __name__ == '__main__':
    appKey = 'app id' 
    appSecret = 'secret'

    BaiduTranslateObj = BaiDuTranslate(appKey, appSecret)
    df = pd.read_excel('language.xlsx')

    englishTexts = df['en'].tolist()
    print(englishTexts)

    chineseTexts = []
    for text in englishTexts:
        translatedTexts = BaiduTranslateObj.Translate(text)
        print(translatedTexts)
        chineseTexts.append(translatedTexts)

    df['cht'] = chineseTexts

    df.to_excel('translated_language.xlsx', index=False)
