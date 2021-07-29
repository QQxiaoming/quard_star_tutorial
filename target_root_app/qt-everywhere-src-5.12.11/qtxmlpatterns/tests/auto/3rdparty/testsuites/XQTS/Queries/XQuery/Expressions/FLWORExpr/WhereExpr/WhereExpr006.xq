(: Name: WhereExpr006 :)
(: Description: For+Where+Return - In the inner For+Where+Return, uses outer variable in 'Where' expr :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context/MyComputer/Drive4//Folder
return 
<Folder>{ $folder/@name }{ for $file in $input-context/MyComputer/Drive4//File
                           where $file/@idref = $folder/@id
                           return 
                           <File>{ $file/@name }</File>
}</Folder>

