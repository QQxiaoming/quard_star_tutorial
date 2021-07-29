(: Name: fn-lang-10:)
(: Description: Evaluation of the fn:lang function with testlang set to "EN". :)
(: Context node is '<para xml:lang="en-us"/>' :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[3]
   return $x/fn:lang("EN")