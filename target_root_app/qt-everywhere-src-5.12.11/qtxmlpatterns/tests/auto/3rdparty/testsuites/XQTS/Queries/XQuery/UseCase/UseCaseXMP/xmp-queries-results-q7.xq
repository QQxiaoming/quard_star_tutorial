(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<bib>
  {
    for $b in $input-context//book
    where $b/publisher = "Addison-Wesley" and $b/@year > 1991
    order by exactly-one($b/title)
    return
        <book>
            { $b/@year }
            { $b/title }
        </book>
  }
</bib> 
