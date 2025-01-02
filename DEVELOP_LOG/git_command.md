##  

## Git 教程

https://www.liaoxuefeng.com/wiki/896043488029600



```
echo "# MediaGuardCmakeV3" >> README.md
git init
git add README.md
git commit -m "first commit"
git branch -M main
git remote add origin git@github.com:lawtatfaitony/MediaGuardCmakeV3.git
git push -u origin main
```

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

```
git remote add origin git@github.com:lawtatfaitony/MediaGuardCmakeV3.git
git branch -M main
git push -u origin main
```

SVN（Apache Subversion）和 Git 都是版本控制系統，但它們之間有一些重要的區別：

1. **分散式 vs 集中式**：
   - Git 是一個分散式版本控制系統，每個開發者都有一個完整的存儲庫複本，可以在本地進行提交、分支和合併等操作。
   - SVN 是一個集中式版本控制系統，所有代碼的存儲和管理都在中央服務器上進行，開發者需要從中央服務器獲取代碼並提交更改。

2. **性能**：
   - Git 在性能方面通常比 SVN 更優秀，特別是在處理大型存儲庫和分支時。
   - SVN 在處理較小項目時可能會更加穩定和可預測。

3. **分支和合併**：
   - Git 的分支和合併功能非常強大且輕鬆，開發者可以輕鬆創建、合併和管理分支。
   - SVN 的分支和合併功能相對較為繁瑣，需要較多的手動操作。

4. **歷史記錄**：
   - Git 存儲歷史記錄的方式更加高效，每次提交都會生成快照，可以輕鬆查看歷史變更。
   - SVN 則是以文件基礎進行版本控制，存儲的是每個文件的變化。

5. **用戶界面**：
   - Git 的命令行界面通常被認為比 SVN 更複雜，但也有許多圖形化的 Git 客戶端可供使用。
   - SVN 的命令行界面相對簡單直觀。

總的來說，Git 更適合大型項目和分布式團隊，提供更靈活、高效的分支和合併功能；而 SVN 在某些情況下可能更容易上手且穩定。選擇使用哪種版本控制系統取決於項目需求、團隊結構和開發流程。

具體命令:

git commit -m "Update deleteFile message tips" File.h