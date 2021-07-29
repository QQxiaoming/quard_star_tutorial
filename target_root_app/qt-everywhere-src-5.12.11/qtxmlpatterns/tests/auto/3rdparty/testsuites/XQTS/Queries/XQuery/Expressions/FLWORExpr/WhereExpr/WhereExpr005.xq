(: Name: WhereExpr005 :)
(: Description: For+Where+Return - use always-false-predicate in 'Where' expr :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<empty>
  { for $folder in $input-context/MyComputer/Drive2//Folder
    where 1 = 2
    return 
    <Folder>
    { $folder/FolderName/text() }
    </Folder> 
  }
</empty>
