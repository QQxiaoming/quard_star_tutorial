(: Name: instanceof2 :)
(: purpose: Evaluation of "instance of" expression for pattern "dateTime instance of xs:time".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:dateTime("2002-04-02T12:00:00Z") instance of xs:time
