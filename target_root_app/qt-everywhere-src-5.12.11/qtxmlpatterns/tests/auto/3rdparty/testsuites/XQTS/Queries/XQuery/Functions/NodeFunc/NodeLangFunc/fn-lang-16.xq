(: Name: fn-lang-16:)
(: Description: Evaluation of the fn:lang function with testlang set to "us-en" and context node :)
(: has xml:lang attribute set to "en-us". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[3]
   return $x/fn:lang("us-us")