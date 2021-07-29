(: Name: instanceof31 :)
(: purpose: Evaluation of "instance of" expression for pattern "date instance of xs:decimal".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:date("2000-01-01+05:00") instance of xs:decimal
