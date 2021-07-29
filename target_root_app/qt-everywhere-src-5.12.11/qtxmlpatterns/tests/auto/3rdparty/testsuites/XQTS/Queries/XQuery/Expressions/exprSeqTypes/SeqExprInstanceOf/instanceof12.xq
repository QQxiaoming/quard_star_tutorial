(: Name: instanceof12 :)
(: purpose: Evaluation of "instance of" expression for pattern "time instance of xs:dateTime".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:time("13:20:10.5Z") instance of xs:dateTime
