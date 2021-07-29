(: Name: defaultnamespacedeclerr-2:)
(: Description: Evaluation of the of a query prolog with two default namespace declarations.:)

declare default function namespace "http://example.org/names";
declare default function namespace "http://someexample.org/names";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"abc"