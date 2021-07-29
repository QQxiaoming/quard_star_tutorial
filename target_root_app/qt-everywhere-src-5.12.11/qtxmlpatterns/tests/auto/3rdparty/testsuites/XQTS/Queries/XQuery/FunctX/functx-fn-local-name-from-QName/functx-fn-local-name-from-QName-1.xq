(:**************************************************************:)
(: Test: functx-fn-local-name-from-QName-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(local-name-from-QName(
   QName('http://datypic.com/prod', 'number')))
