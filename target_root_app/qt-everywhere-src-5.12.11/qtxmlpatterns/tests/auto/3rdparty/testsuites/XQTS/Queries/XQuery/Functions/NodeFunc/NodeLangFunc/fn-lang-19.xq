(: Name: fn-lang-19:)
(: Description: Evaluation of the fn:lang function with testlang set to "fr" and specified node :)
(: (second argument) has xml:lang attribute set to "en". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lang("fr",fn:exactly-one($input-context1/langs/para[1]))