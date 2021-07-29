(: Name: fn-collection-8 :)
(: Description: Return the first title element in each document.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $d in fn:collection($input-context)
return ($d//title)[1]
