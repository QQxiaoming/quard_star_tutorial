(: Name: predicatesns-14:)
(: Description: Evaluation of a simple predicate, that uses the "xs:date()" function. :)
(: Not schema dependent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/date[xs:date(.) = xs:date("2000-01-01+05:00")])