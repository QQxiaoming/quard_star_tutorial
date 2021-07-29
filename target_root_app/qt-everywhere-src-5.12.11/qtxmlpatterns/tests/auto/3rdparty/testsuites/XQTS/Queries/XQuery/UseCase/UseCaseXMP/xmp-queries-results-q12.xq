(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<bib>
{
    for $book1 in $input-context//book,
        $book2 in $input-context//book
    let $aut1 := for $a in $book1/author 
                 order by exactly-one($a/last), exactly-one($a/first)
                 return $a
    let $aut2 := for $a in $book2/author 
                 order by exactly-one($a/last), exactly-one($a/first)
                 return $a
    where $book1 << $book2
    and not($book1/title = $book2/title)
    and deep-equal($aut1, $aut2) 
    return
        <book-pair>
            { $book1/title }
            { $book2/title }
        </book-pair>
}
</bib> 
