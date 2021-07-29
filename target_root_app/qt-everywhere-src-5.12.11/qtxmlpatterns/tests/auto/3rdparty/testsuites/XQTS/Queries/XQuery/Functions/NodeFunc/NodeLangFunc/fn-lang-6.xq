(: Name: fn-lang-6:)
(: Description: Evaluation of the fn:lang function with testlang set to "En". :)
(: Context node is '<para xml:lang="EN"/>' :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[2]
   return $x/fn:lang("En")