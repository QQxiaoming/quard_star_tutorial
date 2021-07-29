(:*******************************************************:)
(: Test: K2-SeqIntersect-42                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Perform intersection between two singleton document nodes. :)
(:*******************************************************:)
declare function local:function ($c as node())
{
    $c intersect $c
};
empty(local:function(document{()}))