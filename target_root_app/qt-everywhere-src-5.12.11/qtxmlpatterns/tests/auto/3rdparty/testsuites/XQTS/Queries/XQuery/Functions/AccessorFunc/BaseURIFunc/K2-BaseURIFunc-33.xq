(:*******************************************************:)
(: Test: K2-BaseURIFunc-33                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure recursive resolution works.           :)
(:*******************************************************:)
<e xml:base="http://example.com/ABC/">
    <a xml:base="../">
        <b xml:base="DEF/file.test"/>
    </a>
</e>/a/b/base-uri()