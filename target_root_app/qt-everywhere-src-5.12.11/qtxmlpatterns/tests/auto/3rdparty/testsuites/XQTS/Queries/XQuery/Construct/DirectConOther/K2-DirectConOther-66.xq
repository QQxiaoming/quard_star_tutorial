(:*******************************************************:)
(: Test: K2-DirectConOther-66                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure direct element constructors makes namespaces in scope for other node constructors. :)
(:*******************************************************:)
declare namespace a="http://example.com/NotThis";
         declare namespace g="http://example.com/NotThis2";
<a xmlns:a="http://example.com/NotThis"
            xmlns:b="http://example.com">
    <b xmlns:a="http://example.com/" xmlns:c="http://example.com/c">
        <c xmlns:d="http://example.com/d"/>
        {
            for $i in in-scope-prefixes(<e/>) order by $i return $i,
            "|",
            for $i in in-scope-prefixes(element e {()}) order by $i return $i
        }
        <d xmlns:e="http://example.com/d"/>
    </b>
</a>