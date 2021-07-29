(:*******************************************************:)
(: Test: K2-NamespaceProlog-8                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Undeclare the 'xdt' prefix.                  :)
(:*******************************************************:)

declare namespace xdt = "";
xdt:untypedAtomic("string")
          