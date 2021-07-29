(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<results>
  {
    for $b in $input-context/bib/book,
        $t in $b/title,
        $a in $b/author
    return
        <result>
            { $t }    
            { $a }
        </result>
  }
</results>
