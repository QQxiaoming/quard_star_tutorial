(:*******************************************************:)
(: Test: K2-NamespaceProlog-10                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Undeclare the 'local' prefix.                :)
(:*******************************************************:)

declare namespace local = "";
local:untypedAtomic("string")
          