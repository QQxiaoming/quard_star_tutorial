(: Name: fn-lang-14:)
(: Description: Evaluation of the fn:lang function with testlang set to "en-us". :)
(: Context node is '<para xml:lang="en-us"/>' :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[3]
   return $x/fn:lang("en-us")