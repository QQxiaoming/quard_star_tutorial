(: Name: fn-lang-17:)
(: Description: Evaluation of the fn:lang function with testlang set to "fr" and context node :)
(: has xml:lang attribute set to "EN". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[2]
   return $x/fn:lang("fr")