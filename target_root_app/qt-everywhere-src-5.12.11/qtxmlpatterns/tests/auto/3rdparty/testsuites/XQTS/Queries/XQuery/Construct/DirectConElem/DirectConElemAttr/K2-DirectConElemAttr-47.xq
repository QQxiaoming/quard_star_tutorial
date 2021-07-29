(:*******************************************************:)
(: Test: K2-DirectConElemAttr-47                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the typed value of directly constructed attributes has the correct type. :)
(:*******************************************************:)
data(<e foo="content"/>/@*) instance of xs:untypedAtomic