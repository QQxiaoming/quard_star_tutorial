(: Name: instanceof14 :)
(: purpose: Evaluation of "instance of" expression for pattern "time instance of xs:date".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:time("13:20:10.5Z") instance of xs:date
