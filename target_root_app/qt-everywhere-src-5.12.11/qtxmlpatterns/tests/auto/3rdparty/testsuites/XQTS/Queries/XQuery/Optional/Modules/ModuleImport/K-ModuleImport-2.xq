(:*******************************************************:)
(: Test: K-ModuleImport-2                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: Module import with empty target namespace, and two location hints. :)
(:*******************************************************:)
import(::)module(::) "" at "http://example.com/", "http://example.com/2"; 1 eq 1