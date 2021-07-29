(: Name: fn-lang-7:)
(: Description: Evaluation of the fn:lang function with testlang set to "eN". :)
(: Context node is '<para xml:lang="EN"/>' :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[2]
   return $x/fn:lang("eN")