(:**************************************************************:)
(: Test: functx-fn-QName-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(QName('http://datypic.com/prod','product'), QName('http://datypic.com/prod',
  'pre:product'), QName('', 'product'))