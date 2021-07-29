(: Name: predicates-24:)
(: Description: Evaluation of a simple predicate, that uses the "!=" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]//employee[empnum != "E1" and empnum != "E4"])