(: Name: fn-normalize-unicode-2 :)
(: Description: Evaluation of fn:normalize-unicode with a normalization form that attempts to raise a non-implemented form:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:normalize-unicode("è","chancesareyoudonotsupportthis123ifyoudowaoo")