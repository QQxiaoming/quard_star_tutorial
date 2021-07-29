(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-15                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Functions cannot occur before namespace declarations. :)
(:*******************************************************:)
declare function local:myFunc()
{
1
};
declare default element namespace "http://example.com";
1