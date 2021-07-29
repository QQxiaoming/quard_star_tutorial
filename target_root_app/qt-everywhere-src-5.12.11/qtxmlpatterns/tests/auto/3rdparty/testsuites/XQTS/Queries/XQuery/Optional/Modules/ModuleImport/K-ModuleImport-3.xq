(:*******************************************************:)
(: Test: K-ModuleImport-3                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: ':=' cannot be used to assing namespaces in 'import module'. :)
(:*******************************************************:)
import module namespace NCName := "http://example.com/Dummy"; 1