(: Name: fn-iri-to-uri-11 :)
(: Description: Examines that the fn:iri-to-uri function does not escape the "(" symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:iri-to-uri("example(example")