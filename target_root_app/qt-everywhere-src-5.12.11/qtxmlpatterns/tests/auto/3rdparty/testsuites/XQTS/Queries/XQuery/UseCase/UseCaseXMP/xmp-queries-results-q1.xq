(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<bib>
 {
  for $b in $input-context/bib/book
  where $b/publisher = "Addison-Wesley" and $b/@year > 1991
  return <book year="{ $b/@year }">{ $b/title }</book>
 }
</bib> 