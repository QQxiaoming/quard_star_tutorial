(: Name: instanceof32 :)
(: purpose: Evaluation of "instance of" expression for pattern "date instance of xs:integer".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:date("2000-01-01+05:00") instance of xs:integer
