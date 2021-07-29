(:**************************************************************:)
(: Test: functx-fn-not-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(not(32 > 20), not(doc($input-context1)
   //product), not(true()), not(()), not(''), not(0), not(<e>false</e>))