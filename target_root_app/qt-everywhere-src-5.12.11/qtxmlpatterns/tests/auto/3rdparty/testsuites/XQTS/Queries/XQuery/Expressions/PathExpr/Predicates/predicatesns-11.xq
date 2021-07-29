(: Name: predicatesns-11:)
(: Description: Evaluation of a simple predicate, that uses the "xs:float()" function. :)
(: Not Schema dependent .:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/float[xs:float(.) = xs:float(1267.43233E12)])