(: Name: fn-replace-9:)
(: Description: Evaluation of replace function with pattern = "^(.*?)d(.*)" and replacement = "$1c$2" as an example 9 for this function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

replace("darted", "^(.*?)d(.*)$", "$1c$2")