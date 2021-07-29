(: Name: combiningnodeseqintersecthc3 :)
(: Description: Simple combination of sequences, where the two sequences are the same.  Use "intersect" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in (
($input-context1//hours) intersect ($input-context1//hours[xs:integer(.) gt 12]))
order by number($h)
return $h
