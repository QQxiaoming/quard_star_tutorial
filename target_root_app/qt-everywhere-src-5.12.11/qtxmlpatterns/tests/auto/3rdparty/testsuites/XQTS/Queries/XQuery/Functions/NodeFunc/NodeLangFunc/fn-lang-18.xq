(: Name: fn-lang-18:)
(: Description: Evaluation of the fn:lang function with testlang set to "en" and specified node :)
(: (second argument) has xml:lang attribute set to "en". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lang("en",fn:exactly-one($input-context1/langs/para[1]))