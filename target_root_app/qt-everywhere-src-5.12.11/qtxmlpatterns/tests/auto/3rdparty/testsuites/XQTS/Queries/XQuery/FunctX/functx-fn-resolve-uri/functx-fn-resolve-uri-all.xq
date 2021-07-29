(:**************************************************************:)
(: Test: functx-fn-resolve-uri-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(resolve-uri('prod', 'http://datypic.com/'), resolve-uri('prod2',
          'http://datypic.com/prod1'), resolve-uri(
   'http://example.org','http://datypic.com'), resolve-uri(
   'http://datypic.com', '../base'), resolve-uri(
   '', 'http://datypic.com'))