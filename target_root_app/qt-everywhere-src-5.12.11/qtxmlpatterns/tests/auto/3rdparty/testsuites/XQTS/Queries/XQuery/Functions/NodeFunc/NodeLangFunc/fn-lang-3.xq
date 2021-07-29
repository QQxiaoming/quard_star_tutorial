(: Name: fn-lang-3:)
(: Description: Evaluation of the fn:lang function with testlang set to "en" as per example 1 for this function:)
(: Context node is 'para xml:lang="en"' :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/para[1]
   return $x/fn:lang("en")