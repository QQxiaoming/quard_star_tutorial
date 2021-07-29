(: Name: fn-replace-2:)
(: Description: Evaluation of replace function with pattern = "a.*a" and replacement = "*" as an example 2 for this function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
replace("abracadabra", "a.*a", "*") 