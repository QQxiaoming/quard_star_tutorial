(:**************************************************************:)
(: Test: functx-fn-iri-to-uri-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(iri-to-uri(
   'http://datypic.com/Sales Numbers.pdf'))
