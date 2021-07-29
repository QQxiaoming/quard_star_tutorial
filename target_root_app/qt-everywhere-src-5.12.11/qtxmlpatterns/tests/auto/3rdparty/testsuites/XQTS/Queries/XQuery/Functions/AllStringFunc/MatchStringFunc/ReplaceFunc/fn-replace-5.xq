(: Name: fn-replace-5:)
(: Description: Evaluation of replace function with pattern = "a(.)" and replacement = "a$1$1" as an example 5 for this function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

replace("abracadabra", "a(.)", "a$1$1")