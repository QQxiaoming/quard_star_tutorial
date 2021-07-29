(: Name: defaultnamespcedeclerr-1:)
(: Description: Evaluation of the of a query prolog with two default namespace declarations.:)

declare default element namespace "http://example.org/names";
declare default element namespace "http://someexample.org/names";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"abc"