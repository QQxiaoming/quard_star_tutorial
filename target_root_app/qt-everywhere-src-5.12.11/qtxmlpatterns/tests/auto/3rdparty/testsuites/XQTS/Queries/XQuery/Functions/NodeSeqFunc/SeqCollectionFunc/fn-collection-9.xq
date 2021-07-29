(: Name: fn-collection-9 :)
(: Description: Count the title elements in each document.:)

(: insert-start :)
(: insert-end :)

for $d in fn:collection()
order by count($d//title)
return count($d//title)
