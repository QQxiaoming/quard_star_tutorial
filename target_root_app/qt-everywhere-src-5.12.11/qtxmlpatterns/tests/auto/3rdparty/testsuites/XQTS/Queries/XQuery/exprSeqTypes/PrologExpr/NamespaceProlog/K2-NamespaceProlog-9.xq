(:*******************************************************:)
(: Test: K2-NamespaceProlog-9                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Undeclare the 'xs' prefix.                   :)
(:*******************************************************:)

declare namespace xs = "";
xs:untypedAtomic("string")
          