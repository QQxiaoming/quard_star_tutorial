(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<results>
{
    for $b in $input-context/bib/book
    return
        <result>
            { $b/title }
            { $b/author  }
        </result>
}
</results> 