(: Name: ForExpr006 :)
(: Description: For+Return - 2 For will generate two iterations :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<fragment-result>{
	for $folder in $input-context/MyComputer/Drive3/Folder
	   ,$file in $input-context/MyComputer/Drive3/Folder/File
	return 
	<Folder>{ $folder/@name, $folder/@id }<file>{ $file/@idref }{ data($file/@name) }</file>
	</Folder>
}</fragment-result>

