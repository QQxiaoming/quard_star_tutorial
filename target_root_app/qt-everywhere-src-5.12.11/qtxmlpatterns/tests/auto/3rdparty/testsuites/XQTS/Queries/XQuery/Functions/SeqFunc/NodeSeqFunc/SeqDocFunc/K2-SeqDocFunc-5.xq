(:*******************************************************:)
(: Test: K2-SeqDocFunc-5                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:doc with a relative, Unix-like URI and a declared base-uri. :)
(:*******************************************************:)
declare base-uri "http://www.example.com";
            doc("/example.com/example.org/does/not/exist/doesNotExist/works-mod.xml")