(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<bib>
  {
    for $b in $input-context//book
    where count($b/author) > 0
    return
        <book>
            { $b/title }
            {
                for $a in $b/author[position()<=2]  
                return $a
            }
            {
                if (count($b/author) > 2)
                 then <et-al/>
                 else ()
            }
        </book>
  }
</bib>