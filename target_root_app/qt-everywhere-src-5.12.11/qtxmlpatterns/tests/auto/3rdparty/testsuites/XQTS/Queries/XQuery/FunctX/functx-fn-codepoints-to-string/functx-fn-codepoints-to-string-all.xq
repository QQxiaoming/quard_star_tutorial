(:**************************************************************:)
(: Test: functx-fn-codepoints-to-string-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(codepoints-to-string((97, 32, 98, 32, 99)), codepoints-to-string(97), codepoints-to-string(()))