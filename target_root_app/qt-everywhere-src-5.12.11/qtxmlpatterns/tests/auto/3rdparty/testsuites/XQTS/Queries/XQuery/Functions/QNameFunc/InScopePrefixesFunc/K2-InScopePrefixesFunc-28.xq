(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-28                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check in-scope declaration of a tree fragment overriding and undeclaration the default namespace. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/";
let $i := <e>
    <a xmlns=""/>
    <b xmlns="http://www.example.com/"/>
    <c xmlns="http://www.example.com/Second"/>
</e>
return (count(in-scope-prefixes($i)),
        count(in-scope-prefixes(exactly-one($i/*[namespace-uri() eq ""]))),
        count(in-scope-prefixes(exactly-one($i/b))),
        count(in-scope-prefixes(exactly-one($i/*[namespace-uri() eq "http://www.example.com/Second"]))),
        $i)