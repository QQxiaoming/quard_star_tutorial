(: Name: fn-replace-6:)
(: Description: Evaluation of replace function with pattern = ".*?" and replacement = "$1" as an example 6 for this function.  Should raise an error:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

replace("abracadabra", ".*?", "$1") 