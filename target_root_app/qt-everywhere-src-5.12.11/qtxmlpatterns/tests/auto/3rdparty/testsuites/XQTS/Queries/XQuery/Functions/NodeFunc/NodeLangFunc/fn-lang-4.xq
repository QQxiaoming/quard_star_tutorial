(: Name: fn-lang-4:)
(: Description: Evaluation of the fn:lang function with testlang set to "en" as per example 2 for this function:)
(: Context node is '<div xml:lang="en"><para>And now, and forever!</para></div>' :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/langs/div[1]/para
   return $x/fn:lang("en")