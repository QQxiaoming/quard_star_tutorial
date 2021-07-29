(:*******************************************************:)
(: Test: K2-NamespaceProlog-12                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Undeclare the 'xsi' prefix.                  :)
(:*******************************************************:)

declare namespace xsi = "";
xsi:untypedAtomic("string")
          