(: Name: WhereExpr007 :)
(: Description: For+Where+Return - 2 iterations use 'Where' to build relationship :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<fragment-result>{
	for $folder in $input-context/MyComputer/Drive3/Folder
	   ,$file in $input-context/MyComputer/Drive3/Folder/File
	where $folder/@id = $file/@idref
	return
	<Folder>
	 { $folder/@name, $folder/@id }
	 <file>{ $file/@idref, $file/FileName/text() }</file>
	</Folder>
}</fragment-result>
