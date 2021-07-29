(: Name: fn-replace-12:)
(: Description: Evaluate that calling function with flags ommited is same as flags being the zero length string. :)


(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

replace("abracadabra", "bra", "*", "")