(:**************************************************************:)
(: Test: functx-fn-resolve-uri-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(resolve-uri('prod2',
          'http://datypic.com/prod1'))
