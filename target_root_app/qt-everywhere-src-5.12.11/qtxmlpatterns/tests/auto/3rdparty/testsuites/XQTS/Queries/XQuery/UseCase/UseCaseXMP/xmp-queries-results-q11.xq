(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<bib>
{
        for $b in $input-context//book[author]
        return
            <book>
                { $b/title }
                { $b/author }
            </book>
}
{
        for $b in $input-context//book[editor]
        return
          <reference>
            { $b/title }
            {$b/editor/affiliation}
          </reference>
}
</bib>  